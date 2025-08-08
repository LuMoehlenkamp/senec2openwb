FROM python:3.11-slim

# Set working directory
WORKDIR /app

# Copy requirements and install dependencies
COPY requirements.txt .
RUN pip install --no-cache-dir -r requirements.txt

# Copy application code
COPY . .

# Create data directory
RUN mkdir -p /app/data

# Set environment variables
ENV PYTHONPATH=/app
ENV DATABASE_URL=sqlite:///./data/resource_management.db

# Expose port for FastAPI
EXPOSE 8000

# Create non-root user
RUN addgroup --system appgroup && adduser --system --group appuser
RUN chown -R appuser:appgroup /app
USER appuser

# Default command (can be overridden)
CMD ["python", "new_main.py"]